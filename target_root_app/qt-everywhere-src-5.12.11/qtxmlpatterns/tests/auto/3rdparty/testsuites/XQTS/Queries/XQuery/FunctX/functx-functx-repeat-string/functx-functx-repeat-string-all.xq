(:**************************************************************:)
(: Test: functx-functx-repeat-string-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Repeats a string a given number of times 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_repeat-string.html 
 : @param   $stringToRepeat the string to repeat 
 : @param   $count the desired number of copies 
 :) 
declare function functx:repeat-string 
  ( $stringToRepeat as xs:string? ,
    $count as xs:integer )  as xs:string {
       
   string-join((for $i in 1 to $count return $stringToRepeat),
                        '')
 } ;
(functx:repeat-string('*', 6), functx:repeat-string('abc', 3))