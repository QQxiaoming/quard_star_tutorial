(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-4               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P11DT12H04M") div xs:dayTimeDuration("P02DT07H01M"))