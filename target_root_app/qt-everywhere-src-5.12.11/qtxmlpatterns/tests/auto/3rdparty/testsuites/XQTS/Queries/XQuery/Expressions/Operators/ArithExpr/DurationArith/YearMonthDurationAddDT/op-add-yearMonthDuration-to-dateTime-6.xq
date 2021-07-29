(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-dateTime-6           :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-dateTime" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dateTime("1988-01-28T13:45:23Z") + xs:yearMonthDuration("P09Y02M"))