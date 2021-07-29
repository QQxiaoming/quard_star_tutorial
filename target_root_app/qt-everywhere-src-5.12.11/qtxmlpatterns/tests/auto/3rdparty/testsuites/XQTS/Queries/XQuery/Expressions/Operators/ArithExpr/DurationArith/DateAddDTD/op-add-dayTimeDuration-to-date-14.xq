(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-date-14                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-date" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:date("1979-12-12Z") + xs:dayTimeDuration("P08DT08H05M")) ne xs:date("1979-12-12Z")