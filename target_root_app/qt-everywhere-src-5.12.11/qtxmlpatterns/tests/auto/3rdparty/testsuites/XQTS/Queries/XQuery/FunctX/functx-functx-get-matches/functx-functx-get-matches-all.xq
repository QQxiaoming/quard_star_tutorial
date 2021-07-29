(:**************************************************************:)
(: Test: functx-functx-get-matches-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Splits a string into matching and non-matching regions 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_get-matches-and-non-matches.html 
 : @param   $string the string to split 
 : @param   $regex the pattern 
 :) 
declare function functx:get-matches-and-non-matches 
  ( $string as xs:string? ,
    $regex as xs:string )  as element()* {
       
   let $iomf := functx:index-of-match-first($string, $regex)
   return
   if (empty($iomf))
   then <non-match>{$string}</non-match>
   else
   if ($iomf > 1)
   then (<non-match>{substring($string,1,$iomf - 1)}</non-match>,
         functx:get-matches-and-non-matches(
            substring($string,$iomf),$regex))
   else
   let $length :=
      string-length($string) -
      string-length(functx:replace-first($string, $regex,''))
   return (<match>{substring($string,1,$length)}</match>,
           if (string-length($string) > $length)
           then functx:get-matches-and-non-matches(
              substring($string,$length + 1),$regex)
           else ())
 } ;

(:~
 : Return the matching regions of a string 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_get-matches.html 
 : @param   $string the string to split 
 : @param   $regex the pattern 
 :) 
declare function functx:get-matches 
  ( $string as xs:string? ,
    $regex as xs:string )  as xs:string* {
       
   functx:get-matches-and-non-matches($string,$regex)/
     string(self::match)
 } ;

(:~
 : The first position of a matching substring 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_index-of-match-first.html 
 : @param   $arg the string 
 : @param   $pattern the pattern to match 
 :) 
declare function functx:index-of-match-first 
  ( $arg as xs:string? ,
    $pattern as xs:string )  as xs:integer? {
       
  if (matches($arg,$pattern))
  then string-length(tokenize($arg, $pattern)[1]) + 1
  else ()
 } ;

(:~
 : Replaces the first match of a pattern 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_replace-first.html 
 : @param   $arg the entire string to change 
 : @param   $pattern the pattern of characters to replace 
 : @param   $replacement the replacement string 
 :) 
declare function functx:replace-first 
  ( $arg as xs:string? ,
    $pattern as xs:string ,
    $replacement as xs:string )  as xs:string {
       
   replace($arg, concat('(^.*?)', $pattern),
             concat('$1',$replacement))
 } ;
(functx:get-matches(
   'abc123def', '\d+'), functx:get-matches(
   'abc123def', '\d'), functx:get-matches(
   'abc123def', '[a-z]{2}'))