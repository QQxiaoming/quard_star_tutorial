(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-8             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-dayTimeDuration-to-dateTime" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:dateTime("0001-01-01T11:11:11Z") + xs:dayTimeDuration("-P11DT02H02M"))