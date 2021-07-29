(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-18       :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-date" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:date("1999-10-23Z") - xs:yearMonthDuration("P19Y12M"))) or fn:false()