(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-12                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" operators used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P10DT08H11M") div 2.0)) and (fn:true())