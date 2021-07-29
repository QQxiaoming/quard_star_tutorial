(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-14         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-date" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:date("1979-12-12Z") - xs:dayTimeDuration("P08DT08H05M")) ne xs:date("1979-12-12Z")