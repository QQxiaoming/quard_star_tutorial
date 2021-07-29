(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-5        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-date" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:date("1962-03-12Z") - xs:yearMonthDuration("P10Y01M")))