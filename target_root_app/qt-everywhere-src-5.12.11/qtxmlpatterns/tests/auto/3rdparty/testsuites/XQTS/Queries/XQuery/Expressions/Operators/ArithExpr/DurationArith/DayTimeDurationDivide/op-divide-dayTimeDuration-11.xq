(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-11                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function used  :)
(:together with multiple "div" expressions.              :)
(:*******************************************************:)

(xs:dayTimeDuration("P42DT10H10M") div 2.0) div (xs:dayTimeDuration("P42DT10H10M") div 2.0)