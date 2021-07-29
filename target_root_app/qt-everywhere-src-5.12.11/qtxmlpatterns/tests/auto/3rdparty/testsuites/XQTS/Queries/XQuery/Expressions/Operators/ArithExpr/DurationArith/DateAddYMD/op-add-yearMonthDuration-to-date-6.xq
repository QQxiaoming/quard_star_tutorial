(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-6               :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:date("1988-01-28Z") + xs:yearMonthDuration("P09Y02M"))