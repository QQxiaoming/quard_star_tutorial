(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-8               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H01M") div xs:dayTimeDuration("-P10DT01H01M"))