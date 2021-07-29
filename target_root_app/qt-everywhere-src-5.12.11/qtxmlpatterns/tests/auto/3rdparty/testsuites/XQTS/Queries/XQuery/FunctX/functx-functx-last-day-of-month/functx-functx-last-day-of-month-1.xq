(:**************************************************************:)
(: Test: functx-functx-last-day-of-month-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Construct a date from a year, month and day 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_date.html 
 : @param   $year the year 
 : @param   $month the month 
 : @param   $day the day 
 :) 
declare function functx:date 
  ( $year as xs:anyAtomicType ,
    $month as xs:anyAtomicType ,
    $day as xs:anyAtomicType )  as xs:date {
       
   xs:date(
     concat(
       functx:pad-integer-to-length(xs:integer($year),4),'-',
       functx:pad-integer-to-length(xs:integer($month),2),'-',
       functx:pad-integer-to-length(xs:integer($day),2)))
 } ;

(:~
 : Number of days in the month 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_days-in-month.html 
 : @param   $date the date 
 :) 
declare function functx:days-in-month 
  ( $date as xs:anyAtomicType? )  as xs:integer? {
       
   if (month-from-date(xs:date($date)) = 2 and
       functx:is-leap-year($date))
   then 29
   else
   (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)
    [month-from-date(xs:date($date))]
 } ;

(:~
 : Whether a date falls in a leap year 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-leap-year.html 
 : @param   $date the date or year 
 :) 
declare function functx:is-leap-year 
  ( $date as xs:anyAtomicType? )  as xs:boolean {
       
    for $year in xs:integer(substring(string($date),1,4))
    return ($year mod 4 = 0 and
            $year mod 100 != 0) or
            $year mod 400 = 0
 } ;

(:~
 : The last day of the month of a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_last-day-of-month.html 
 : @param   $date the date 
 :) 
declare function functx:last-day-of-month 
  ( $date as xs:anyAtomicType? )  as xs:date? {
       
   functx:date(year-from-date(xs:date($date)),
            month-from-date(xs:date($date)),
            functx:days-in-month($date))
 } ;

(:~
 : Pads an integer to a desired length by adding leading zeros 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_pad-integer-to-length.html 
 : @param   $integerToPad the integer to pad 
 : @param   $length the desired length 
 :) 
declare function functx:pad-integer-to-length 
  ( $integerToPad as xs:anyAtomicType? ,
    $length as xs:integer )  as xs:string {
       
   if ($length < string-length(string($integerToPad)))
   then error(xs:QName('functx:Integer_Longer_Than_Length'))
   else concat
         (functx:repeat-string(
            '0',$length - string-length(string($integerToPad))),
          string($integerToPad))
 } ;

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
(functx:last-day-of-month(xs:date('2004-01-23')))
