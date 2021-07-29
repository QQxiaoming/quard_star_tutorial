(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-2                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string((xs:dayTimeDuration("P10DT10H11M")) * 2.0) and fn:false()