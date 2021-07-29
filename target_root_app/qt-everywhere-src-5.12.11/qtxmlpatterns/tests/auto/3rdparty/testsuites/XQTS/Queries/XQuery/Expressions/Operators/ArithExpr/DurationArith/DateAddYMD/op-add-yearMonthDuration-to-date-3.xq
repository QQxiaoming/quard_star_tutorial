(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-3               :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "add-yearMonthDuration-to-date" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:date("1999-10-23Z") + xs:yearMonthDuration("P19Y12M"))) or fn:false()