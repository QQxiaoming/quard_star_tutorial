(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-14            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-dateTime" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:dateTime("1979-12-12T18:18:18Z") + xs:dayTimeDuration("P08DT08H05M")) ne xs:dateTime("1979-12-12T16:15:14Z")