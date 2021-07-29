(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-2                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDurations" function :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:dayTimeDuration("P10DT10H11M") - xs:dayTimeDuration("P12DT10H07M")) and fn:false()