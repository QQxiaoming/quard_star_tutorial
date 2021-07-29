(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-time-14                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-time" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:time("04:04:05Z") + xs:dayTimeDuration("P08DT08H05M")) ne xs:time("05:08:02Z")