(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-16              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P13DT09H09M") div xs:dayTimeDuration("P18DT02H02M")) ge  xs:decimal(2.0)