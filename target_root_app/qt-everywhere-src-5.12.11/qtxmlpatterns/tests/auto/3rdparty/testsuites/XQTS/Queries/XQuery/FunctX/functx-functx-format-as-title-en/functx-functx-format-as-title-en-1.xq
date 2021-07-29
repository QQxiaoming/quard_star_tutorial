(:**************************************************************:)
(: Test: functx-functx-format-as-title-en-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Moves title words like "the" and "a" to the end of strings 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_format-as-title-en.html 
 : @param   $titles the titles to format 
 :) 
declare function functx:format-as-title-en 
  ( $titles as xs:string* )  as xs:string* {
       
   let $wordsToMoveToEnd := ('A', 'An', 'The')
   for $title in $titles
   let $firstWord := functx:substring-before-match($title,'\W')
   return if ($firstWord = $wordsToMoveToEnd)
          then replace($title,'(.*?)\W(.*)', '$2, $1')
          else $title
 } ;

(:~
 : The substring before the first text that matches a regex 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-before-match.html 
 : @param   $arg the string to substring 
 : @param   $regex the regular expression 
 :) 
declare function functx:substring-before-match 
  ( $arg as xs:string? ,
    $regex as xs:string )  as xs:string? {
       
   tokenize($arg,$regex)[1]
 } ;
(functx:format-as-title-en(
     ('A Midsummer Night''s Dream',
      'The Merchant of Venice',
      'Hamlet')))
