(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-10              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P05DT09H02M") div xs:dayTimeDuration("P03DT01H04M")) or (xs:dayTimeDuration("P05DT05H03M") div xs:dayTimeDuration("P01DT01H03M"))