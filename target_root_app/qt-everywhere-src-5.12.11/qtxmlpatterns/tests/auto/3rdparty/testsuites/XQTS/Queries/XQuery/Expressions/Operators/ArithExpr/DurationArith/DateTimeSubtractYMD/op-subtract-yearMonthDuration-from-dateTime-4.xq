(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-4    :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-dateTime" :)
(: operator that return true and used together with fn:not.:)
(:*******************************************************:)
 
fn:not(fn:string(xs:dateTime("1998-09-12T13:56:12Z") - xs:yearMonthDuration("P20Y03M")))