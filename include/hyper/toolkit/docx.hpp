/** ////////////////////////////////////////////////////////////////

    *** EZ-C++ - A simplified C++ experience ***

        Yet (another) open source library for C++

        Original Copyright (C) Damian Tran 2019

        By aiFive Technologies, Inc. for developers

    Copying and redistribution of this code is freely permissible.
    Inclusion of the above notice is preferred but not required.

    This software is provided AS IS without any expressed or implied
    warranties.  By using this code, and any modifications and
    variants arising thereof, you are assuming all liabilities and
    risks that may be thus associated.

////////////////////////////////////////////////////////////////  **/

#pragma once

#ifndef EZ_DOCX
#define EZ_DOCX

#include "hyper/toolkit/zip.hpp"

#include <map>
#include <string>

const static size_t ZIP_READ_BUF_SIZE = 100000;

struct MSW_HEADER{

    unsigned char FibBase[32];

    unsigned char*  FibRgW97,
                  *  FibRgLw97,
                  *  FibRgFcLcb,
                  *  fibRgCswNew;

    uint16_t csw,
            cslw,
            cbRgFcLcb,
            cswNew;

    size_t size() const{
        return 32 + 8 + csw*2 + cslw*4 + cbRgFcLcb*8 + cswNew*2;
    }

    // Base block ===========================================================

    uint16_t wIdent()   const{ return *(uint16_t*)&FibBase[0]; }
    uint16_t nFib()     const{ return *(uint16_t*)&FibBase[2]; }
    uint16_t unused()   const{ return *(uint16_t*)&FibBase[4]; }
    uint16_t lid()      const{ return *(uint16_t*)&FibBase[6]; }
    uint16_t pnNext()   const{ return *(uint16_t*)&FibBase[8]; }

    bool     fDot()     const{ return FibBase[10] & 0b1;        }
    bool     fGlsy()    const{ return FibBase[10] & 0b10;       }
    bool     fComplex() const{ return FibBase[10] & 0b100;      }
    bool     fHasPic()  const{ return FibBase[10] & 0b1000;     }

    uint8_t  cQuickSaves() const{
        return uint8_t(FibBase[10]) >> 4;
    }

    bool    fEncrypted()            const{ return FibBase[11] & 0b1;        }
    bool    fWhichTblStm()          const{ return FibBase[11] & 0b10;       }
    bool    fReadOnlyRecommended()  const{ return FibBase[11] & 0b100;      }
    bool    fWriteReservation()     const{ return FibBase[11] & 0b1000;     }
    bool    fExtChar()              const{ return FibBase[11] & 0b10000;    }
    bool    fLoadOverride()         const{ return FibBase[11] & 0b100000;   }
    bool    fFarEast()              const{ return FibBase[11] & 0b1000000;  }
    bool    fObfuscated()           const{ return FibBase[11] & 0b10000000; }

    uint16_t    nFibBack()      const{ return *(uint16_t*)&FibBase[12]; }
    uint32_t    lKey()          const{ return *(uint32_t*)&FibBase[14]; }
    uint8_t     envr()          const{ return uint8_t(FibBase[18]);     }

    bool    fMac()                  const{ return FibBase[19] & 0b1;        }
    bool    fEmptySpecial()         const{ return FibBase[19] & 0b10;       }
    bool    fLoadOverridePage()     const{ return FibBase[19] & 0b100;      }
    bool    reserved1()             const{ return FibBase[19] & 0b1000;     }
    bool    reserved2()             const{ return FibBase[19] & 0b10000;    }
    bool    fSpare0()               const{ return FibBase[19] & 0b11100000; }

    uint16_t    reserved3()     const{ return *(uint16_t*)&FibBase[20]; }
    uint16_t    reserved4()     const{ return *(uint16_t*)&FibBase[22]; }

    uint32_t    reserved5()     const{ return *(uint32_t*)&FibBase[24]; }
    uint32_t    reserved6()     const{ return *(uint32_t*)&FibBase[28]; }

    // FibRgLw97 ==============================================================

    uint32_t    cbMac()         const{
        if(FibRgLw97) return *(uint32_t*)FibRgLw97;
        return 0U;
    }
    uint32_t    ccpText()       const{
        if(FibRgLw97) return *(uint32_t*)(FibRgLw97 + 12);
        return 0U;
    }

    friend std::ostream& operator<<(std::ostream& output, const MSW_HEADER& hdr){
        output << "wIdent:\t"           <<      std::hex << hdr.wIdent()
                << "\nnFib:\t"          <<      hdr.nFib()
                << "\nnFibBack:\t"      <<      hdr.nFibBack()
                << "\ncsw:\t"           <<      std::dec << hdr.csw
                << "\ncslw:\t"          <<      hdr.cslw
                << "\ncbRgFcLb:\t"      <<      hdr.cbRgFcLcb
                << "\ncswNew:\t"        <<      hdr.cswNew
                << "\nBlock size:\t"    <<      hdr.size()
                << "\nDocument size:\t" <<      hdr.cbMac()
                << "\nCharacters:\t"    <<      hdr.ccpText();
        return output;
    }

    template<typename T>
    void read(T* ptr){
        memcpy(FibBase, ptr, 32);
        if(valid()){

            int offset = 32;

            csw = *(uint16_t*)(ptr + offset);
            offset += 2;

            FibRgW97 = new unsigned char[csw*2];
            memcpy(FibRgW97, ptr + offset, csw*2);
            offset += csw*2;

            cslw = *(uint16_t*)(ptr + offset);
            offset += 2;

            FibRgLw97 = new unsigned char[cslw*4];
            memcpy(FibRgLw97, ptr + offset, cslw*4);
            offset += cslw*4;

            cbRgFcLcb = *(uint16_t*)(ptr + offset);
            offset += 2;

            FibRgFcLcb = new unsigned char[cbRgFcLcb*8];
            memcpy(FibRgFcLcb, ptr + offset, cbRgFcLcb*8);
            offset += cbRgFcLcb*8;

            cswNew = *(uint16_t*)(ptr + offset);
            offset += 2;

            if(cswNew){

                fibRgCswNew = new unsigned char[cswNew*2];
                memcpy(fibRgCswNew, ptr + offset, cswNew*2);


            }

        }
    }

    bool valid() const{
        return (wIdent() == 0xA5EC) &&
                fExtChar() && !envr() && !fMac()
                && !reserved3() && !reserved4() &&
                ((nFibBack() == 0x00BF) || (nFibBack() == 0x00C1)) &&
                ((fEncrypted() || fObfuscated()) ^ !lKey());
    }

    MSW_HEADER():
        FibRgW97(NULL),
        FibRgLw97(NULL),
        FibRgFcLcb(NULL),
        fibRgCswNew(NULL),
        csw(0),
        cslw(0){ }

    ~MSW_HEADER(){
        if(FibRgW97)    delete[] FibRgW97;
        if(FibRgLw97)   delete[] FibRgLw97;
        if(FibRgFcLcb)  delete[] FibRgFcLcb;
        if(fibRgCswNew) delete[] fibRgCswNew;
    }

};

bool getDocumentBody(std::string& output, const std::string& file);

typedef std::map<wchar_t, wchar_t> cMap;
bool checkCMap(const char* str);
bool getCMap(cMap& output, const char* str);

#endif // EZ_DOCX
