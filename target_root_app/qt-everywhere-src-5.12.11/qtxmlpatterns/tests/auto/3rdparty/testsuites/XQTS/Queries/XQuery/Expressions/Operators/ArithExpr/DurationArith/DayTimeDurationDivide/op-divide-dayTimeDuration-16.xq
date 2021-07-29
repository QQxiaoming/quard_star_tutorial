(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-16                     :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration" function used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P13DT09H09M") div 2.0) ge  xs:dayTimeDuration("P18DT02H02M")