SELECT CompanyName, ROUND(delayCount * 100.0 / classCount,2) AS delayPercent
FROM 
(SELECT ShipVia, COUNT(*) delayCount FROM 'Order' WHERE ShippedDate > RequiredDate GROUP BY ShipVia) AS classDelayCount 
INNER JOIN
(SELECT ShipVia, COUNT(*) classCount FROM 'Order' GROUP BY ShipVia) AS classAllCount
ON classDelayCount.ShipVia = classAllCount.ShipVia
INNER JOIN
Shipper on classAllCount.ShipVia = Shipper.Id
ORDER BY delayPercent DESC;
