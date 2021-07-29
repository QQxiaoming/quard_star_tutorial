(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-15                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:time("08:09:09Z") + xs:dayTimeDuration("P17DT10H02M")) le xs:time("09:08:10Z")