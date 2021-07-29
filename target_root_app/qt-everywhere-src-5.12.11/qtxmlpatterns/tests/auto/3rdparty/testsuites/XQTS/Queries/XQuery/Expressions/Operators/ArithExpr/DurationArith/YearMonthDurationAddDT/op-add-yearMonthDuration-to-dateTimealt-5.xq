(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-dateTime-5           :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-yearMonthDuration-to-dateTime" :)
(:operator that is used as an argument to the fn:boolean function. :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:dateTime("1962-03-12T10:12:34Z") + xs:yearMonthDuration("P10Y01M")))