(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-date-13         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-date" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:date("1980-05-05Z") - xs:dayTimeDuration("P23DT11H11M")) eq xs:date("1980-05-05Z")