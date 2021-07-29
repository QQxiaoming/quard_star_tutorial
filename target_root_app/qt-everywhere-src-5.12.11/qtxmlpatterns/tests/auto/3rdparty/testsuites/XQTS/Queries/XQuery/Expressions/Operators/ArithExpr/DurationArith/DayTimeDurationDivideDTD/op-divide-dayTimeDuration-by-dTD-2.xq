(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-2               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

xs:dayTimeDuration("P10DT10H11M") div xs:dayTimeDuration("P12DT10H07M") and fn:false()