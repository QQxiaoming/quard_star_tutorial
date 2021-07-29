(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-17       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-yearMonthDuration-from-date" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:date("2000-12-12Z") - xs:yearMonthDuration("P18Y11M")) and fn:false()