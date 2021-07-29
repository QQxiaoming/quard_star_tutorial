(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-12              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operators used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT08H11M") div xs:dayTimeDuration("P05DT08H11M")) and (fn:true())