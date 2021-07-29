(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-12                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" operators used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:dayTimeDuration("P10DT08H11M") * 2.0)) and (fn:true())