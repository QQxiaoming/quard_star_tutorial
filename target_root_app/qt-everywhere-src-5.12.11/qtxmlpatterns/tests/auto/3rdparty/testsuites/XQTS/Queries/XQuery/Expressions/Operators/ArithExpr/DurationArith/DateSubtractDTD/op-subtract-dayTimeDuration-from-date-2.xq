(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-2          :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-date" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:date("2000-12-12Z") - xs:dayTimeDuration("P12DT10H07M")) and fn:false()