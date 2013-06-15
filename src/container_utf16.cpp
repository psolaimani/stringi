/* This file is part of the 'stringi' library.
 * 
 * Copyright 2013 Marek Gagolewski, Bartek Tartanus, Marcin Bujarski
 * 
 * 'stringi' is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * 'stringi' is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with 'stringi'. If not, see <http://www.gnu.org/licenses/>.
 */
 

#include "stringi.h"


/** 
 * Default constructor
 * 
 */
StriContainerUTF16::StriContainerUTF16()
   : StriContainerUTF_Base()
{
   this->str = NULL;
   this->lastMatcher = NULL;
}


/**
 * Construct String Container from R character vector
 * @param rstr R character vector
 * @param nrecycle extend length [vectorization]
 * @param shallowrecycle will \code{this->str} be ever modified?
 */
StriContainerUTF16::StriContainerUTF16(SEXP rstr, R_len_t nrecycle, bool shallowrecycle)
{
   this->lastMatcher = NULL;
   this->str = NULL;
#ifndef NDEBUG 
   if (!isString(rstr))
      error("DEBUG: !isString in StriContainerUTF16::StriContainerUTF16(SEXP rstr)");
#endif
   R_len_t nrstr = LENGTH(rstr);
   this->init_Base(nrstr, nrecycle, shallowrecycle); // calling LENGTH(rstr) fails on constructor call
   

   if (this->n > 0) {
      this->str = new UnicodeString*[this->n];
      for (R_len_t i=0; i<this->n; ++i)
         this->str[i] = NULL; // in case it fails during conversion (this is NA)
      
      UConverter* ucnvASCII = NULL;
//      UConverter* ucnvUTF8 = NULL;
      UConverter* ucnvLatin1 = NULL;
      UConverter* ucnvNative = NULL;

      for (R_len_t i=0; i<nrstr; ++i) {
         SEXP curs = STRING_ELT(rstr, i);
         if (curs == NA_STRING) {
            continue; // keep NA
         }
         else {
            if (IS_ASCII(curs)) {         
               if (!ucnvASCII) ucnvASCII = stri__ucnv_open("ASCII");      
               UErrorCode status = U_ZERO_ERROR;
               this->str[i] = new UnicodeString(CHAR(curs), LENGTH(curs),
                  ucnvASCII, status);
               if (U_FAILURE(status))
                  error(MSG__ENC_ERROR_CONVERT);  
                  
               // Performance improvement attempt #1:
               // this->str[i] = new UnicodeString(UnicodeString::fromUTF8(CHAR(curs))); // slower than the above
               
               // Performance improvement attempt #2:
               // Create UChar buf with LENGTH(curs) items, fill it with (CHAR(curs)[i], 0x00), i=1,...
               // This wasn't faster tham the ucnvASCII approach.
            }
            else if (IS_UTF8(curs)) {
               // the above ASCII-approach (but with ucnvUTF8) is slower for UTF-8
               this->str[i] = new UnicodeString(UnicodeString::fromUTF8(CHAR(curs)));
            }
            else if (IS_LATIN1(curs)) {
               if (!ucnvLatin1) ucnvLatin1 = stri__ucnv_open("ISO-8859-1");
               UErrorCode status = U_ZERO_ERROR;
               this->str[i] = new UnicodeString(CHAR(curs), LENGTH(curs),
                  ucnvLatin1, status);
               if (U_FAILURE(status))
                  error(MSG__ENC_ERROR_CONVERT);  
            }
            else if (IS_BYTES(curs)) 
               error(MSG__BYTESENC);
            else {
//             Any encoding - detection needed
//             Assume it's Native; this assumes the user working in an 8-bit environment
//             would convert strings to UTF-8 manually if needed - I think is's
//             a more reasonable approach (Native --> input via keyboard)
               if (!ucnvNative) ucnvNative = stri__ucnv_open((char*)NULL);
               UErrorCode status = U_ZERO_ERROR;
               this->str[i] = new UnicodeString(CHAR(curs), LENGTH(curs),
                  ucnvNative, status);
               if (U_FAILURE(status))
                  error(MSG__ENC_ERROR_CONVERT);  
            }
         }
      }
      
      if (ucnvASCII) ucnv_close(ucnvASCII);
//      if (ucnvUTF8)  ucnv_close(ucnvUTF8);
      if (ucnvLatin1) ucnv_close(ucnvLatin1);
      if (ucnvNative) ucnv_close(ucnvNative);
      
      if (!shallowrecycle) {
         for (R_len_t i=nrstr; i<this->n; ++i) {
            if (this->str[i%nrstr] == NULL)
               this->str[i] = NULL;
            else
               this->str[i] = new UnicodeString(*this->str[i%nrstr]);
         }
      }
   }
}



/** Copy constructor
 * 
 */
StriContainerUTF16::StriContainerUTF16(StriContainerUTF16& container)
   :    StriContainerUTF_Base((StriContainerUTF_Base&)container)
{
   this->lastMatcher = NULL;
   if (container.str) {
      this->str = new UnicodeString*[this->n];
      for (int i=0; i<this->n; ++i) {
         if (container.str[i])
            this->str[i] = new UnicodeString(*(container.str[i]));
         else
            this->str[i] = NULL;
      }
   }
   else {
      this->str = NULL;
   }
}




StriContainerUTF16& StriContainerUTF16::operator=(StriContainerUTF16& container)
{
   this->~StriContainerUTF16();
   (StriContainerUTF_Base&) (*this) = (StriContainerUTF_Base&)container;
   
   this->lastMatcher = NULL;
   if (container.str) {
      this->str = new UnicodeString*[this->n];
      for (int i=0; i<this->n; ++i) {
         if (container.str[i])
            this->str[i] = new UnicodeString(*(container.str[i]));
         else
            this->str[i] = NULL;
      }
   }
   else {
      this->str = NULL;
   }
   return *this;
}


/** Destructor
 * 
 */
StriContainerUTF16::~StriContainerUTF16()
{
   if (lastMatcher) {
      delete lastMatcher;
      lastMatcher = NULL;
   }

   if (str) {
      for (int i=0; i<this->n; ++i) {
         if (str[i])
            delete str[i];
      }
      delete [] str;
      str = NULL;
   }
}




/** Export character vector to R
 *  THE OUTPUT IS ALWAYS IN UTF-8
 *  Recycle rule is applied, so length == nrecycle
 * @return STRSXP
 */
SEXP StriContainerUTF16::toR() const
{
   SEXP ret;   
   PROTECT(ret = allocVector(STRSXP, nrecycle));
   std::string buf;
   
   for (R_len_t i=0; i<nrecycle; ++i) {
      if (!str[i%n])
         SET_STRING_ELT(ret, i, NA_STRING);
      else {
         buf.clear();
         str[i%n]->toUTF8String(buf);
         SET_STRING_ELT(ret, i,
            mkCharLenCE(buf.c_str(), buf.length(), CE_UTF8));
      }
   }
   
   UNPROTECT(1);
   return ret;
}



/** Export string to R
 *  THE OUTPUT IS ALWAYS IN UTF-8
 *  @param i index [with recycle]
 *  @return CHARSXP
 */
SEXP StriContainerUTF16::toR(R_len_t i) const
{
#ifndef NDEBUG
   if (i < 0 || i >= nrecycle) error("StriContainerUTF16::toR(): INDEX OUT OF BOUNDS");
#endif

   if (str[i%n] == NULL)
      return NA_STRING;
   else {
      std::string s;
      str[i%n]->toUTF8String(s);
      return mkCharLenCE(s.c_str(), s.length(), CE_UTF8);
   }
}
 
 

/** the returned matcher shall not be deleted by the user 
 * 
 * it is assumed that \code{vectorize_next()} is used:
 * for \code{i >= this->n} the last matcher is returned
 * 
 * @param i index
 */
RegexMatcher* StriContainerUTF16::vectorize_getMatcher(R_len_t i)
{
   if (lastMatcher) {
      if (i >= n) {
#ifndef NDEBUG
      if ((debugMatcherIndex % n) != (i % n)) {
         error("DEBUG: vectorize_getMatcher - matcher reuse failed!");
      }
#endif
         return lastMatcher; // reuse
      }
      else {
         delete lastMatcher;
         lastMatcher = NULL;
      }
   }
   
   UErrorCode status = U_ZERO_ERROR;
   lastMatcher = new RegexMatcher(this->get(i), 0, status);
   if (U_FAILURE(status)) {
      error(u_errorName(status));
   }
#ifndef NDEBUG
   debugMatcherIndex = (i % n);
#endif

   return lastMatcher;
}