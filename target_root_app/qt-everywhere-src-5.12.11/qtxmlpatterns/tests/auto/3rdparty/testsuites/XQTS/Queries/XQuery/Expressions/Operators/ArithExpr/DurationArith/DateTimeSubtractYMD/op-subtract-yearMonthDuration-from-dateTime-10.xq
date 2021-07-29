(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-10   :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-dateTime" :)
(: operator used together with an "or" expression.       :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1985-07-05T09:09:09Z") - xs:yearMonthDuration("P02Y02M"))) or fn:string((xs:dateTime("1985-07-05T09:09:09Z") - xs:yearMonthDuration("P02Y02M")))