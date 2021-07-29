(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-4               :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "add-yearMonthDuration-to-date" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:date("1998-09-12Z") + xs:yearMonthDuration("P20Y03M")))