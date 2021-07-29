(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-2    :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-dateTime" :)
(: operator used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:dateTime("2000-12-12T12:12:12Z") - xs:dayTimeDuration("P12DT10H07M")) and fn:false()