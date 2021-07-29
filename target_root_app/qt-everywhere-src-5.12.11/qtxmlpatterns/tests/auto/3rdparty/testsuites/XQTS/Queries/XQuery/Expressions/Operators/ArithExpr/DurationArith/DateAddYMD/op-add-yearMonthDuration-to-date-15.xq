(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-15              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:date("1978-12-12Z") + xs:yearMonthDuration("P17Y12M")) le xs:date("1978-12-12Z")