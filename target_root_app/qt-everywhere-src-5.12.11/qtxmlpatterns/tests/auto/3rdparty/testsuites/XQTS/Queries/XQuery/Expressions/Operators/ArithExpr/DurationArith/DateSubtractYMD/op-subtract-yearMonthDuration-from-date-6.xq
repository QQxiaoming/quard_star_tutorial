(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-6        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:date("1988-01-28Z") - xs:yearMonthDuration("P09Y02M"))