(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-15                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-date" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:date("1978-12-12Z") + xs:dayTimeDuration("P17DT10H02M")) le xs:date("1978-12-12Z")