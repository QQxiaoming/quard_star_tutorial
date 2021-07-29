(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-11                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:dayTimeDuration("P42DT10H10M") * 2.0) div (xs:dayTimeDuration("P42DT10H10M") * 2.0)