(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-15     :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-dateTime" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:dateTime("1978-12-12T12:45:12Z") - xs:dayTimeDuration("P17DT10H02M")) le xs:dateTime("1978-12-12T16:34:23Z")