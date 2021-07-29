(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-dateTime-9           :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-yearMonthDuration-to-dateTime" :)
(:operator used together with an "and" expression.       :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1993-12-09T10:10:10Z") + xs:yearMonthDuration("P03Y03M"))) and fn:string((xs:dateTime("1993-12-09T10:10:10Z") + xs:yearMonthDuration("P03Y03M")))