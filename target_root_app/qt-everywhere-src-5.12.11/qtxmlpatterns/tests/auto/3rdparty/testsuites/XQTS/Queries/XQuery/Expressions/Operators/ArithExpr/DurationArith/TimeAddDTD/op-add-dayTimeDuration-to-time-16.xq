(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-16                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:time("09:06:07Z") + xs:dayTimeDuration("P18DT02H02M")) ge  xs:time("01:01:01Z")