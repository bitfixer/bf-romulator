#include "zm.h"
#include "zmodem.h"
#include <LittleFS.h>
#include <Arduino.h>

#define DATA_BUF_LEN    2048
#define PRINTF(...) printf(__VA_ARGS__)
#define FPRINTF(...) fprintf(__VA_ARGS__)

ZRESULT zm_recv() {
  int res;
  do {
    res = Serial.read();
  } while (res < 0);
  return res;
}

ZRESULT zm_send(uint8_t chr) {
  Serial.write(chr);
  return OK;
}

void zmodemRecvFile(const char* fname) {
  uint8_t rzr_buf[4];
  uint8_t data_buf[DATA_BUF_LEN];
  uint16_t count;
  uint32_t received_data_size = 0;
  ZHDR hdr;
  File fp = LittleFS.open(fname, "w");

#ifdef ZDEBUG_DUMP_BAD_BLOCKS
  uint32_t bad_block_count = 0;
#endif

  //if ((com = init_com(argc, argv)) != NULL) {
    if (true) {
    DEBUGF("Opened port just fine\n");

    PRINTF("rosco_m68k ZMODEM receive example v0.01 - Awaiting remote transfer initiation...\n");

    if (zm_await("rz\r", (char*)rzr_buf, 4) == ZOK) {
      DEBUGF("Got rzr...\n");

      while (true) {
startframe:
        DEBUGF("\n====================================\n");
        uint16_t result = zm_await_header(&hdr);

        switch (result) {
        case CANCELLED:
          FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
          goto cleanup;
        case ZOK:
          DEBUGF("Got valid header\n");

          switch (hdr.type) {
          case ZRQINIT:
          case ZEOF:
            DEBUGF("Is ZRQINIT or ZEOF\n");

            result = zm_send_flags_hdr(ZRINIT, CANOVIO | CANFC32, 0, 0, 0);

            if (result == CANCELLED) {
              FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
              goto cleanup;
            } else if (result == ZOK) {
              DEBUGF("Send ZRINIT was ZOK\n");
            } else if(result == CLOSED) {
              FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
              goto cleanup;
            }

            continue;

          case ZFIN:
            DEBUGF("Is ZFIN\n");

            result = zm_send_pos_hdr(ZFIN, 0);

            if (result == CANCELLED) {
              FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
              goto cleanup;
            } else if (result == ZOK) {
              DEBUGF("Send ZFIN was ZOK\n");
            } else if(result == CLOSED) {
              FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
            }

            PRINTF("Transfer complete; Received %0d byte(s)\n", received_data_size);
            goto cleanup;

          case ZFILE:
            DEBUGF("Is ZFILE\n");

            switch (hdr.flags.f0) {
            case 0:     /* no special treatment - default to ZCBIN */
            case ZCBIN:
              DEBUGF("--> Binary receive\n");
              break;
            case ZCNL:
              DEBUGF("--> ASCII Receive; Fix newlines (IGNORED - NOT SUPPORTED)\n");
              break;
            case ZCRESUM:
              DEBUGF("--> Resume interrupted transfer (IGNORED - NOT SUPPORTED)\n");
              break;
            default:
              FPRINTF(stderr, "WARN: Invalid conversion flag [0x%02x] (IGNORED - Assuming Binary)\n", hdr.flags.f0);
            }

            count = DATA_BUF_LEN;
            result = zm_read_data_block(data_buf, &count);
            DEBUGF("Result of data block read is [0x%04x] (got %d character(s))\n", result, count);

            if (result == CANCELLED) {
              FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
              goto cleanup;
            } else if (!IS_ERROR(result)) {
              PRINTF("Receiving file: '%s'\n", data_buf);

              /*
              out = fopen((char*)data_buf, "wb");
              if (out == NULL) {
                FPRINTF(stderr, "Error opening file for output; Bailing...\n");
                goto cleanup;
              }
              */

              result = zm_send_pos_hdr(ZRPOS, received_data_size);

              if (result == CANCELLED) {
                FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
                goto cleanup;
              } else if (result == ZOK) {
                  DEBUGF("Send ZRPOS was ZOK\n");
              } else if(result == CLOSED) {
                  FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
                  goto cleanup;
              }
            }

            // TODO care about XON that will follow?

            continue;

          case ZDATA:
            DEBUGF("Is ZDATA\n");

            while (true) {
              count = DATA_BUF_LEN;
              result = zm_read_data_block(data_buf, &count);
              DEBUGF("Result of data block read is [0x%04x] (got %d character(s))\n", result, count);

              /*
              if (out == NULL) {
                FPRINTF(stderr, "Received data before open file; Bailing...\n");
                goto cleanup;
              }
              */

              if (result == CANCELLED) {
                FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
                goto cleanup;
              } else if (!IS_ERROR(result)) {
                DEBUGF("Received %d byte(s) of data\n", count);

                //fwrite(data_buf, count - 1, 1, out);
                fp.write(data_buf, count-1);
                received_data_size += (count - 1);

                if (result == GOT_CRCE) {
                  // End of frame, header follows, no ZACK expected.
                  DEBUGF("Got CRCE; Frame done [NOACK] [Pos: 0x%08x]\n", received_data_size);
                  break;
                } else if (result == GOT_CRCG) {
                  // Frame continues, non-stop (another data packet follows)
                  DEBUGF("Got CRCG; Frame continues [NOACK] [Pos: 0x%08x]\n", received_data_size);
                  continue;
                } else if (result == GOT_CRCQ) {
                  // Frame continues, ZACK required
                  DEBUGF("Got CRCQ; Frame continues [ACK] [Pos: 0x%08x]\n", received_data_size);

                  result = zm_send_pos_hdr(ZACK, received_data_size);

                  if (result == CANCELLED) {
                    FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
                    goto cleanup;
                  } else if (result == ZOK) {
                    DEBUGF("Send ZACK was ZOK\n");
                  } else if(result == CLOSED) {
                    FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
                    goto cleanup;
                  }

                  continue;
                } else if (result == GOT_CRCW) {
                  // End of frame, header follows, ZACK expected.
                  DEBUGF("Got CRCW; Frame done [ACK] [Pos: 0x%08x]\n", received_data_size);

                  result = zm_send_pos_hdr(ZACK, received_data_size);

                  if (result == CANCELLED) {
                    FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
                    goto cleanup;
                  } else if (result == ZOK) {
                    DEBUGF("Send ZACK was ZOK\n");
                  } else if(result == CLOSED) {
                    FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
                    goto cleanup;
                  }

                  break;
                }

              } else {
                DEBUGF("Error while receiving block: 0x%04x\n", result);

                result = zm_send_pos_hdr(ZRPOS, received_data_size);

#ifdef ZDEBUG_DUMP_BAD_BLOCKS
                char name[20];
                snprintf(name, 20, "block%d.bin", bad_block_count++);
                DEBUGF("  >> Writing file '%s'\n", name);
                FILE *block = fopen(name, "wb");
                fwrite(data_buf,count,1,block);
                fclose(block);
#endif

                if (result == CANCELLED) {
                  FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
                  goto cleanup;
                } else if (result == ZOK) {
                  DEBUGF("Send ZRPOS was ZOK\n");
                  goto startframe;
                } else if(result == CLOSED) {
                  FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
                  goto cleanup;
                }
              }
            }

            continue;

          default:
            PRINTF("WARN: Ignoring unknown header type 0x%02x\n", hdr.type);
            continue;
          }

          break;
        case BAD_CRC:
          DEBUGF("Didn't get valid header - CRC Check failed\n");

          result = zm_send_pos_hdr(ZNAK, received_data_size);

          if (result == CANCELLED) {
            FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
            goto cleanup;
          } else if (result == ZOK) {
            DEBUGF("Send ZNACK was ZOK\n");
          } else if(result == CLOSED) {
            FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
            goto cleanup;
          }

          continue;
        default:
          DEBUGF("Didn't get valid header - result is 0x%04x\n", result);

          result = zm_send_pos_hdr(ZNAK, received_data_size);

          if (result == CANCELLED) {
            FPRINTF(stderr, "Transfer cancelled by remote; Bailing...\n");
            goto cleanup;
          } else if (result == ZOK) {
            DEBUGF("Send ZNACK was ZOK\n");
          } else if(result == CLOSED) {
            FPRINTF(stderr, "Connection closed prematurely; Bailing...\n");
            goto cleanup;
          }

          continue;
        }
      }
    } else {
      PRINTF("zm_await failed\n");
    }

    cleanup:
    fp.close();
    /*
    if (out != NULL && fclose(out)) {
      FPRINTF(stderr, "Failed to close output file\n");
    }
    if (com != NULL && fclose(com)) {
      FPRINTF(stderr, "Failed to close serial port\n");
    }
    */

  }
}