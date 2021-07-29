(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-13                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P23DT11H11M") * 2.0) eq xs:dayTimeDuration("P23DT11H11M")