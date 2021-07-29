(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-16     :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-dateTime" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dateTime("1977-12-12T13:12:15Z") - xs:dayTimeDuration("P18DT02H02M")) ge  xs:dateTime("1977-12-12T15:56:10Z")