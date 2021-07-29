(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-9    :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates string value of the "subtract-yearMonthDuration-from-dateTime" :)
(:operator used together with an "and" expression.       :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1993-12-09T10:10:10Z") - xs:yearMonthDuration("P03Y03M"))) and fn:string((xs:dateTime("1993-12-09T10:10:10Z") - xs:yearMonthDuration("P03Y03M")))