(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-13         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-time" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:time("01:03:03Z") - xs:dayTimeDuration("P23DT11H11M")) eq xs:time("04:03:05Z")