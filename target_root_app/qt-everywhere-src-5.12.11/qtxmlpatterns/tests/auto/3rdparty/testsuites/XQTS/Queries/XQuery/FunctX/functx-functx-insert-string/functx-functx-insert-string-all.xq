(:**************************************************************:)
(: Test: functx-functx-insert-string-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Inserts a string at a specified position 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_insert-string.html 
 : @param   $originalString the original string to insert into 
 : @param   $stringToInsert the string to insert 
 : @param   $pos the position 
 :) 
declare function functx:insert-string 
  ( $originalString as xs:string? ,
    $stringToInsert as xs:string? ,
    $pos as xs:integer )  as xs:string {
       
   concat(substring($originalString,1,$pos - 1),
             $stringToInsert,
             substring($originalString,$pos))
 } ;
(functx:insert-string('xyz','def',2), functx:insert-string('xyz','def',5), functx:insert-string('xyz','',2), functx:insert-string('','def',2))