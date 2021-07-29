(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-15       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:date("1978-12-12Z") - xs:yearMonthDuration("P17Y12M")) le xs:date("1978-12-12Z")