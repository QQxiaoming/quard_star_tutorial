(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-13            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-dateTime" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:dateTime("1980-05-05T17:17:17Z") + xs:dayTimeDuration("P23DT11H11M")) eq xs:dateTime("1980-05-05T17:17:17Z")