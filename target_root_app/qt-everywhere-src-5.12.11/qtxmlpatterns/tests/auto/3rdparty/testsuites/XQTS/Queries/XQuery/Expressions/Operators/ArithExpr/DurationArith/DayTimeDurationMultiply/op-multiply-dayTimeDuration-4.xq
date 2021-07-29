(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-4                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:dayTimeDuration("P11DT12H04M") * 2.0))