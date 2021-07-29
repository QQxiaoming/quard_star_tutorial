(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-13              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P23DT11H11M") div xs:dayTimeDuration("P23DT11H11M")) eq xs:decimal(2.0)