(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-4                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:dayTimeDuration("P11DT12H04M") div 2.0))