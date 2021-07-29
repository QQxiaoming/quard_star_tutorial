(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-12                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" operators used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P10DT08H11M") - xs:dayTimeDuration("P05DT08H11M"))) and (fn:true())