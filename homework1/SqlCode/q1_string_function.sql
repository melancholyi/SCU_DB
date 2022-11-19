SELECT DISTINCT ShipName , SUBSTR(ShipName, 0 , instr(ShipName,'-')) as preShipName
FROM 'Order'
WHERE ShipName LIKE '%-%'
ORDER BY ShipName;

