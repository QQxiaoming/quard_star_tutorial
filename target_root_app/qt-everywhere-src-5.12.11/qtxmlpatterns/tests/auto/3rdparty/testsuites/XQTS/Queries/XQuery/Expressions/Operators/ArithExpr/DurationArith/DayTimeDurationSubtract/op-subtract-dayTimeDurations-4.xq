(:*******************************************************:)
(:Test: op-subtract-dayTimeDurations-4                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-dayTimeDurations" function that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:dayTimeDuration("P11DT12H04M") - xs:dayTimeDuration("P02DT07H01M")))