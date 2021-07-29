(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-12   :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-dateTime" :)
(:operator used with a boolean expression and the "fn:true" function. :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1980-03-02T02:02:02Z") - xs:yearMonthDuration("P05Y05M"))) and (fn:true())