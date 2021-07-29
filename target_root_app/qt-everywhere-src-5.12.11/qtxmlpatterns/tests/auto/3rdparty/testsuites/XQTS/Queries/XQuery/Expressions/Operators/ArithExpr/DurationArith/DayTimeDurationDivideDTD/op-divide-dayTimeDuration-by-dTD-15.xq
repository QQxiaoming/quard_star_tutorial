(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-15              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-DTD" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT10H01M") div xs:dayTimeDuration("P17DT10H02M")) le xs:decimal(2.0)