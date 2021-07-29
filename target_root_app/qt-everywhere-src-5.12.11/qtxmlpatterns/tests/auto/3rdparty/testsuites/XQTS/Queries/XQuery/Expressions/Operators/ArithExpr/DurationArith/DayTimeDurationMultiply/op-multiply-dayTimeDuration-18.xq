(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-18                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function :)
(:multiplied by +0. Use fn:count to avoid empty file.    :)
(:*******************************************************:)
 
fn:count(xs:dayTimeDuration("P13DT09H09M") * +0)