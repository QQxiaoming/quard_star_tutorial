(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-16                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P13DT09H09M") * 2.0) ge  xs:dayTimeDuration("P18DT02H02M")